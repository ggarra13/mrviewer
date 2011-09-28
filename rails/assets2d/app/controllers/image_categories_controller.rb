class ImageCategoriesController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @image_category_pages, @image_categories = paginate :image_categories, :per_page => 10
  end

  def show
    @image_category = ImageCategory.find(params[:id])
  end

  def new
    @image_category = ImageCategory.new
  end

  def create
    @image_category = ImageCategory.new(params[:image_category])
    if @image_category.save
      flash[:notice] = 'ImageCategory was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @image_category = ImageCategory.find(params[:id])
  end

  def update
    @image_category = ImageCategory.find(params[:id])
    if @image_category.update_attributes(params[:image_category])
      flash[:notice] = 'ImageCategory was successfully updated.'
      redirect_to :action => 'show', :id => @image_category
    else
      render :action => 'edit'
    end
  end

  def destroy
    ImageCategory.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
