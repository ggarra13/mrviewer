require File.dirname(__FILE__) + '/../test_helper'
require 'render_transforms_controller'

# Re-raise errors caught by the controller.
class RenderTransformsController; def rescue_action(e) raise e end; end

class RenderTransformsControllerTest < Test::Unit::TestCase
  fixtures :render_transforms

  def setup
    @controller = RenderTransformsController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = render_transforms(:first).id
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

    assert_not_nil assigns(:render_transforms)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:render_transform)
    assert assigns(:render_transform).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:render_transform)
  end

  def test_create
    num_render_transforms = RenderTransform.count

    post :create, :render_transform => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_render_transforms + 1, RenderTransform.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:render_transform)
    assert assigns(:render_transform).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      RenderTransform.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      RenderTransform.find(@first_id)
    }
  end
end
