require File.dirname(__FILE__) + '/../test_helper'
require 'image_categories_controller'

# Re-raise errors caught by the controller.
class ImageCategoriesController; def rescue_action(e) raise e end; end

class ImageCategoriesControllerTest < Test::Unit::TestCase
  fixtures :image_categories

  def setup
    @controller = ImageCategoriesController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = image_categories(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:image_categories)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:image_category)
    assert assigns(:image_category).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:image_category)
  end

  def test_create
    num_image_categories = ImageCategory.count

    post :create, :image_category => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_image_categories + 1, ImageCategory.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:image_category)
    assert assigns(:image_category).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      ImageCategory.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      ImageCategory.find(@first_id)
    }
  end
end
